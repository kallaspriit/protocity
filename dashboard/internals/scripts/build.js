/* eslint-disable no-console, import/no-dynamic-require, global-require */

import chalk from 'chalk';
import fs from 'fs-extra';
import path from 'path';
import webpack from 'webpack';
import checkRequiredFiles from 'react-dev-utils/checkRequiredFiles';
import FileSizeReporter from 'react-dev-utils/FileSizeReporter';
import config from '../webpack/webpack.prod.babel';
import paths from '../paths';

process.on('unhandledRejection', (err) => {
	throw err;
});

// https://github.com/motdotla/dotenv
require('dotenv').config({ silent: true });

const measureFileSizesBeforeBuild = FileSizeReporter.measureFileSizesBeforeBuild;
const printFileSizesAfterBuild = FileSizeReporter.printFileSizesAfterBuild;

// Warn and crash if required files are missing
if (!checkRequiredFiles([paths.indexHtml, paths.indexJs])) {
	process.exit(1);
}


// Print out errors
function printErrors(summary, errors) {
	console.log(chalk.red(summary));
	console.log();
	errors.forEach((err) => {
		console.log(err.message || err);
		console.log();
	});
}

// Create the production build and print the deployment instructions.
function build(previousFileSizes) {
	console.log('Creating an optimized production build...');

	let compiler;
	try {
		compiler = webpack(config);
	} catch (err) {
		printErrors('Failed to compile.', [err]);
		process.exit(1);
	}

	compiler.run((err, stats) => {
		if (err) {
			printErrors('Failed to compile.', [err]);
			process.exit(1);
		}

		if (stats.compilation.errors.length) {
			printErrors('Failed to compile.', stats.compilation.errors);
			process.exit(1);
		}

		if (process.env.CI && stats.compilation.warnings.length) {
			printErrors(
				'Failed to compile. When process.env.CI = true, warnings are treated as failures.',
				stats.compilation.warnings,
			);
			process.exit(1);
		}

		console.log(chalk.green('Compiled successfully.'));
		console.log();

		console.log('File sizes after gzip:');
		console.log();
		printFileSizesAfterBuild(stats, previousFileSizes);
		console.log();

		console.log(`The ${chalk.cyan(path.relative(process.cwd(), paths.build))} folder is ready to be deployed.`);
		console.log('You may serve it with a static server:');
		console.log();

		console.log(`  ${chalk.cyan('npm')} install -g serve`);
		console.log(`  ${chalk.cyan('serve')} -s build`);
		console.log();
	});
}

function copyPublicFolder() {
	fs.copySync(paths.public, paths.build, {
		dereference: true,
		filter: file => file !== paths.indexHtml,
	});
}


measureFileSizesBeforeBuild(paths.build).then((previousFileSizes) => {
	// Remove all content but keep the directory so that
	// if you're in it, you don't end up in Trash
	fs.emptyDirSync(paths.build);

	// Start the webpack build
	build(previousFileSizes);

	// Merge with the public folder
	copyPublicFolder();
});