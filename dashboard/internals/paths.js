import path from 'path';
import fs from 'fs';

const appDirectory = fs.realpathSync(process.cwd());
const resolveApp = relativePath => path.resolve(appDirectory, relativePath);

export default {
	root: appDirectory,
	src: resolveApp('src'),
	public: resolveApp('public'),
	build: resolveApp('build'),
	config: resolveApp('config'),
	nodeModules: resolveApp('node_modules'),
	indexJs: resolveApp('src/index.js'),
	indexHtml: resolveApp('public/index.html'),
	packageJson: resolveApp('package.json'),
	env: resolveApp('.env'),
};