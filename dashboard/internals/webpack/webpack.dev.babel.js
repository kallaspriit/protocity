import webpack from 'webpack';
import HtmlWebpackPlugin from 'html-webpack-plugin';
import CaseSensitivePathsPlugin from 'case-sensitive-paths-webpack-plugin';
import WatchMissingNodeModulesPlugin from 'react-dev-utils/WatchMissingNodeModulesPlugin';
import paths from '../paths';
import webpackBase from './webpack.base.babel';

export default (webpackBase)({
	cache: true,
	devtool: process.env.devTool || 'cheap-module-source-map',

	output: {
		pathinfo: true,
	},

	entry: [
		require.resolve('react-hot-loader/patch'),
		// require.resolve('webpack-dev-server/client') + '?/',
		// require.resolve('webpack/hot/dev-server'),
		require.resolve('react-dev-utils/webpackHotDevClient'),
		require.resolve('../../config/polyfills'),
		paths.indexJs,
	],

	plugins: [
		new HtmlWebpackPlugin({
			inject: true,
			template: paths.indexHtml,
		}),
		new webpack.HotModuleReplacementPlugin(),
		new webpack.NamedModulesPlugin(),
		new webpack.NoEmitOnErrorsPlugin(),
		new CaseSensitivePathsPlugin(),
		new WatchMissingNodeModulesPlugin(paths.nodeModules),
	],
});
