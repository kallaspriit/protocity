/* eslint-disable no-console, import/no-dynamic-require, global-require */

import chalk from 'chalk';
import webpack from 'webpack';
import WebpackDevServer from 'webpack-dev-server';
import historyApiFallback from 'connect-history-api-fallback';
import httpProxyMiddleware from 'http-proxy-middleware';
import detect from 'detect-port';
import clearConsole from 'react-dev-utils/clearConsole';
import checkRequiredFiles from 'react-dev-utils/checkRequiredFiles';
import formatWebpackMessages from 'react-dev-utils/formatWebpackMessages';
import getProcessForPort from 'react-dev-utils/getProcessForPort';
import prompt from 'react-dev-utils/prompt';
import config from '../webpack/webpack.dev.babel';
import paths from '../paths';

const isInteractive = process.stdout.isTTY;
// Warn and crash if required files are missing
if (!checkRequiredFiles([paths.indexHtml, paths.indexJs])) {
	process.exit(1);
}

// Tools like Cloud9 rely on this.
const DEFAULT_PORT = process.env.PORT || parseInt(process.env.APP_PORT, 10) || 3000;
let compiler;
let handleCompile;

// You can safely remove this after ejecting.
// We only use this block for testing of Create React App itself:
const isSmokeTest = process.argv.some(arg => arg.indexOf('--smoke-test') > -1);
if (isSmokeTest) {
	handleCompile = (err, stats) => {
		if (err || stats.hasErrors() || stats.hasWarnings()) {
			process.exit(1);
		} else {
			process.exit(0);
		}
	};
}

function setupCompiler(host, port, protocol) {
  // "Compiler" is a low-level interface to Webpack.
  // It lets us listen to some events and provide our own custom messages.
	compiler = webpack(config, handleCompile);

  // "invalid" event fires when you have changed a file, and Webpack is
  // recompiling a bundle. WebpackDevServer takes care to pause serving the
  // bundle, so if you refresh, it'll wait instead of serving the old one.
  // "invalid" is short for "bundle invalidated", it doesn't imply any errors.
	compiler.plugin('invalid', () => {
		if (isInteractive) {
			clearConsole();
		}
		console.log('Compiling...');
	});

	let isFirstCompile = true;

  // "done" event fires when Webpack has finished recompiling the bundle.
  // Whether or not you have warnings or errors, you will get this event.
	compiler.plugin('done', (stats) => {
		if (isInteractive) {
			clearConsole();
		}

    // We have switched off the default Webpack output in WebpackDevServer
    // options so we are going to "massage" the warnings and errors and present
    // them in a readable focused way.
		const messages = formatWebpackMessages(stats.toJson({}, true));
		const isSuccessful = !messages.errors.length && !messages.warnings.length;
		const showInstructions = isSuccessful && (isInteractive || isFirstCompile);

		if (isSuccessful) {
			console.log(chalk.green('Compiled successfully!'));
		}

		if (showInstructions) {
			console.log();
			console.log('The app is running at:');
			console.log();
			console.log(`  ${chalk.cyan(`${protocol}://${host}:${port}/`)}`);
			console.log();
			console.log('Note that the development build is not optimized.');
			console.log(`To create a production build, use ${chalk.cyan('yarn run build')}.`);
			console.log();
			isFirstCompile = false;
		}

    // If errors exist, only show errors.
		if (messages.errors.length) {
			console.log(chalk.red('Failed to compile.'));
			console.log();
			messages.errors.forEach((message) => {
				console.log(message);
				console.log();
			});
			return;
		}

    // Show warnings if no errors were found.
		if (messages.warnings.length) {
			console.log(chalk.yellow('Compiled with warnings.'));
			console.log();
			messages.warnings.forEach((message) => {
				console.log(message);
				console.log();
			});
      // Teach some ESLint tricks.
			console.log('You may use special comments to disable some warnings.');
			console.log(`Use ${chalk.yellow('// eslint-disable-next-line')} to ignore the next line.`);
			console.log(`Use ${chalk.yellow('/* eslint-disable */')} to ignore all warnings in a file.`);
		}
	});
}

// We need to provide a custom onError function for httpProxyMiddleware.
// It allows us to log custom error messages on the console.
function onProxyError(proxy) {
	return (err, req, res) => {
		const host = req.headers && req.headers.host;
		console.log(
      `${chalk.red('Proxy error:')} Could not proxy request ${chalk.cyan(req.url)
      } from ${chalk.cyan(host)} to ${chalk.cyan(proxy)}.`,
    );
		console.log(
      `See https://nodejs.org/api/errors.html#errors_common_system_errors for more information (${
      chalk.cyan(err.code)}).`,
    );
		console.log();

    // And immediately send the proper error response to the client.
    // Otherwise, the request will eventually timeout with ERR_EMPTY_RESPONSE on the client side.
		if (res.writeHead && !res.headersSent) {
			res.writeHead(500);
		}
		res.end(`Proxy error: Could not proxy request ${req.url} from ${
      host} to ${proxy} (${err.code}).`,
    );
	};
}

function addMiddleware(devServer) {
  // `proxy` lets you to specify a fallback server during development.
  // Every unrecognized request will be forwarded to it.
	const proxy = require(paths.packageJson).proxy;
	devServer.use(historyApiFallback({
    // Paths with dots should still use the history fallback.
    // See https://github.com/facebookincubator/create-react-app/issues/387.
		disableDotRule: true,
    // For single page apps, we generally want to fallback to /index.html.
    // However we also want to respect `proxy` for API calls.
    // So if `proxy` is specified, we need to decide which fallback to use.
    // We use a heuristic: if request `accept`s text/html, we pick /index.html.
    // Modern browsers include text/html into `accept` header when navigating.
    // However API calls like `fetch()` won’t generally accept text/html.
    // If this heuristic doesn’t work well for you, don’t use `proxy`.
		htmlAcceptHeaders: proxy ?
      ['text/html'] :
      ['text/html', '*/*'],
	}));
	if (proxy) {
		if (typeof proxy !== 'string') {
			console.log(chalk.red('When specified, "proxy" in package.json must be a string.'));
			console.log(chalk.red(`Instead, the type of "proxy" was "${typeof proxy}".`));
			console.log(chalk.red('Either remove "proxy" from package.json, or make it a string.'));
			process.exit(1);
		}

    // Otherwise, if proxy is specified, we will let it handle any request.
    // There are a few exceptions which we won't send to the proxy:
    // - /index.html (served as HTML5 history API fallback)
    // - /*.hot-update.json (WebpackDevServer uses this too for hot reloading)
    // - /sockjs-node/* (WebpackDevServer uses this for hot reloading)
    // Tip: use https://jex.im/regulex/ to visualize the regex
		const mayProxy = /^(?!\/(index\.html$|.*\.hot-update\.json$|sockjs-node\/)).*$/;

    // Pass the scope regex both to Express and to the middleware for proxying
    // of both HTTP and WebSockets to work without false positives.
		const hpm = httpProxyMiddleware(pathname => mayProxy.test(pathname), {
			target: proxy,
			logLevel: 'silent',
			onProxyReq(proxyReq) {
        // Browers may send Origin headers even with same-origin
        // requests. To prevent CORS issues, we have to change
        // the Origin to match the target URL.
				if (proxyReq.getHeader('origin')) {
					proxyReq.setHeader('origin', proxy);
				}
			},
			onError: onProxyError(proxy),
			secure: false,
			changeOrigin: true,
			ws: true,
		});
		devServer.use(mayProxy, hpm);

    // Listen for the websocket 'upgrade' event and upgrade the connection.
    // If this is not done, httpProxyMiddleware will not try to upgrade until
    // an initial plain HTTP request is made.
		devServer.listeningApp.on('upgrade', hpm.upgrade);
	}

  // Finally, by now we have certainly resolved the URL.
  // It may be /index.html, so let the dev server try serving it again.
	devServer.use(devServer.middleware);
}

function runDevServer(host, port, protocol) {
	const devServer = new WebpackDevServer(compiler, {
		compress: true,
		clientLogLevel: 'none',
		contentBase: paths.public,
		hot: true,
		publicPath: config.output.publicPath,
		quiet: true,
		watchOptions: {
			ignored: /node_modules/,
		},
		https: protocol === 'https',
		host,
	});

  // Our custom middleware proxies requests to /index.html or a remote API.
	addMiddleware(devServer);

  // Launch WebpackDevServer.
	devServer.listen(port, (err) => {
		if (err) {
			console.log(err);
			return;
		}

		if (isInteractive) {
			clearConsole();
		}
		console.log(chalk.cyan('Starting the development server...'));
		console.log();
	});
}

function run(port) {
	const protocol = process.env.HTTPS === 'true' ? 'https' : 'http';
	const host = process.env.HOST || 'localhost';
	setupCompiler(host, port, protocol);
	runDevServer(host, port, protocol);
}

// We attempt to use the default port but if it is busy, we offer the user to
// run on a different port. `detect()` Promise resolves to the next free port.
detect(DEFAULT_PORT).then((port) => {
	if (port === DEFAULT_PORT) {
		run(port);
		return;
	}

	if (isInteractive) {
		clearConsole();
		const existingProcess = getProcessForPort(DEFAULT_PORT);
		const question =
      `${chalk.yellow(`Something is already running on port ${DEFAULT_PORT}.${
        (existingProcess) ? ` Probably:\n  ${existingProcess}` : ''}`)
        }\n\nWould you like to run the app on another port instead?`;

		prompt(question, true).then((shouldChangePort) => {
			if (shouldChangePort) {
				run(port);
			}
		});
	} else {
		console.log(chalk.red(`Something is already running on port ${DEFAULT_PORT}.`));
	}
});
