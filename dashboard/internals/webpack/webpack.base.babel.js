import webpack from 'webpack';
import autoprefixer from 'autoprefixer';
import InterpolateHtmlPlugin from 'react-dev-utils/InterpolateHtmlPlugin';
import dotenv from 'dotenv';
import paths from '../paths';
import getClientEnvironment from '../../config/env';

// Load environment variables from .env file. Suppress warnings using silent
// if this file is missing. dotenv will never modify any environment variables
// that have already been set.
// https://github.com/motdotla/dotenv
dotenv.config({ silent: true });

const includedPaths = [
	paths.src,
	paths.config,
];

const publicUrl = '';
const env = getClientEnvironment(publicUrl);

export default options => ({
	entry: options.entry,

	output: {
		path: paths.build,
		publicPath: '/',
		filename: '[name].js',
		chunkFilename: '[name].js',
		...options.output,
	},

	module: {
		rules: [{
			test: /\.js$/,
			include: includedPaths,
			loader: 'babel-loader',
			query: {
				cacheDirectory: true,
			},
		}, {
			test: /\.scss/,
			include: includedPaths,
			use: [
				'style-loader',
				{
					loader: 'css-loader',
					options: { sourceMap: true, importLoaders: 1, fixUrls: true },
				},
				{
					loader: 'postcss-loader',
					options: {
						plugins: () => ([
							autoprefixer({
								browsers: [
									'>1%',
									'last 4 versions',
									'Firefox ESR',
									'not ie < 11',
								],
							}),
						]),
					},
				},
				{
					loader: 'sass-loader',
					query: { outputStyle: 'expanded' },
				},
			],
		}, {
			test: /\.(ttf|woff|woff2)$/,
			loader: 'file-loader',
		}, {
			test: /\.(gif|png|jpe?g|svg)$/,
			loaders: [
				{
					loader: 'file-loader',
					query: {
						hash: 'sha512',
						digest: 'hex',
						name: '[path][name].[ext]',
					},
				},
				{
					loader: 'image-webpack-loader',
					query: {
						progressive: true,
						optipng: {
							optimizationLevel: 4,
						},
						gifsicle: {
							interlaced: false,
						},
						pngquant: {
							quality: '65-90',
							speed: 4,
						},
					},
				},
			],
		}],
	},

	plugins: options.plugins.concat([
		new webpack.DefinePlugin(env),
		new InterpolateHtmlPlugin({
			PUBLIC_URL: publicUrl,
		}),
	]),
	resolve: {
		modules: ['src', 'node_modules'],
		extensions: [
			'.js',
			'.jsx',
			'.react.js',
		],
		mainFields: [
			'browser',
			'jsnext:main',
			'main',
		],
	},
	devtool: options.devtool || 'source-map',
	target: 'web', // Make web variables accessible to webpack, e.g. window
	performance: options.performance || {},
});