import path from 'path';
import webpack from 'webpack';
import HtmlWebpackPlugin from 'html-webpack-plugin';
import ExtractTextPlugin from 'extract-text-webpack-plugin';
import autoprefixer from 'autoprefixer';

const srcPath = path.resolve(__dirname, '..', 'src');
const buildPath = path.resolve(__dirname, '..', 'build');

export default {
	bail: true,
	devtool: 'source-map',
	entry: path.join(srcPath, 'index'),
	output: {
		path: buildPath,
		filename: '[name].[chunkhash].js',
		chunkFilename: '[name].[chunkhash].chunk.js',
		publicPath: '/',
	},

	module: {
		loaders: [{
			test: /\.js$/,
			exclude: /node_modules/,
			loader: 'babel-loader',
			query: {
				plugins: [
					'transform-react-inline-elements',
					'transform-react-constant-elements',
				],
			},
		}, {
			test: /\.scss/,
			loader: ExtractTextPlugin.extract({
				fallbackLoader: 'style',
				loader: ['css?sourceMap!postcss?sourceMap!sass?sourceMap'],
			}),
			exclude: /node_modules/,
		}, {
			test: /\.(jpg|jpeg|gif|png|svg|woff|woff2)$/,
			loader: `url?limit=10000&name=[path][name].[ext]&context=${__dirname}`,
			exclude: /node_modules/,
		}],
	},

	postcss: () => [
		autoprefixer({
			browsers: [
				'>1%',
				'last 4 versions',
				'Firefox ESR',
				'not ie < 9',
			],
		}),
	],

	plugins: [
		new HtmlWebpackPlugin({
			inject: true,
			template: path.join(srcPath, 'index.template.html'),
			favicon: path.join(srcPath, 'gfx/favicon.ico'),
		}),
		new webpack.DefinePlugin({ 'process.env.NODE_ENV': '"production"' }),
		new webpack.optimize.OccurrenceOrderPlugin(),
		new webpack.optimize.DedupePlugin(),
		new ExtractTextPlugin('[name].[contenthash].css'),
		new webpack.optimize.UglifyJsPlugin({
			sourceMap: true,
			compressor: {
				warnings: false,
			},
		}),
	],
};
