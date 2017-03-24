import webpack from 'webpack';
import autoprefixer from 'autoprefixer';
import ExtractTextPlugin from 'extract-text-webpack-plugin';
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

const extractSass = new ExtractTextPlugin({
	filename: '[name].[contenthash:8].css',
	disable: process.env.NODE_ENV === 'development',
});

const postCssOptions = {
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
};

const fileLoaderOptions = {
	hash: 'sha512',
	digest: 'hex',
	name: process.env.NODE_ENV === 'production' ? 'static/gfx/[name].[hash:8].[ext]' : '[path][name].[ext]',
};

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
			use: extractSass.extract({
				fallback: 'style-loader',
				use: [
					{ loader: 'css-loader', options: { sourceMap: true, importLoaders: 1, fixUrls: true } },
					{ loader: 'postcss-loader', options: postCssOptions },
					{ loader: 'sass-loader', query: { outputStyle: 'expanded' } },
				],
			}),
		}, {
			test: /\.(ttf|woff|woff2)$/,
			loader: 'file-loader',
		}, {
			test: /\.(gif|png|jpe?g|svg)$/,
			loaders: [
				{
					loader: 'file-loader',
					query: fileLoaderOptions,
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
		extractSass,
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