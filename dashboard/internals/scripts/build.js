/* eslint-disable no-console, import/first */
/* eslint no-param-reassign: ["error", { "props": false }] */

// Do this as the first thing so that any code reading it knows the right env.
process.env.NODE_ENV = 'production';

import fs from 'fs';
import { sync as rimrafSync } from 'rimraf';
import chalk from 'chalk';
import filesize from 'filesize';
import { sync as gzipSize } from 'gzip-size';
import recursive from 'recursive-readdir';
import path from 'path';
import webpack from 'webpack';
import stripAnsi from 'strip-ansi';
import config from '../webpack/webpack.prod.babel';

const buildPath = path.resolve(__dirname, '../build');

// Input: /User/dan/app/build/static/js/main.82be8.js
// Output: /static/js/main.js
function removeFileNameHash(fileName) {
	return fileName
		.replace(buildPath, '')
		.replace(/\/?(.*)(\.\w+)(\.js|\.css)/, (match, p1, p2, p3) => p1 + p3);
}

// Input: 1024, 2048
// Output: "(+1 KB)"
function getDifferenceLabel(currentSize, previousSize) {
	const FIFTY_KILOBYTES = 1024 * 50;
	const difference = currentSize - previousSize;
	const fileSize = !Number.isNaN(difference) ? filesize(difference) : 0;
	if (difference >= FIFTY_KILOBYTES) {
		return chalk.red(`+${fileSize}`);
	} else if (difference < FIFTY_KILOBYTES && difference > 0) {
		return chalk.yellow(`+${fileSize}`);
	} else if (difference < 0) {
		return chalk.green(fileSize);
	}
	return '';
}

// Print a detailed summary of build files.
function printFileSizes(stats, previousSizeMap) {
	const assets = stats.toJson().assets
		.filter(asset => /\.(js|css)$/.test(asset.name))
		.map((asset) => {
			const fileContents = fs.readFileSync(`${buildPath}/${asset.name}`);
			const size = gzipSize(fileContents);
			const previousSize = previousSizeMap[`\\${removeFileNameHash(asset.name)}`];
			const difference = getDifferenceLabel(size, previousSize);
			return {
				folder: path.join('build', path.dirname(asset.name)),
				name: path.basename(asset.name),
				size,
				sizeLabel: filesize(size) + (difference ? ` (${difference})` : ''),
			};
		});
	assets.sort((a, b) => b.size - a.size);
	const longestSizeLabelLength = Math.max.apply(null,
		assets.map(a => stripAnsi(a.sizeLabel).length),
	);
	assets.forEach((asset) => {
		let sizeLabel = asset.sizeLabel;
		const sizeLength = stripAnsi(sizeLabel).length;
		if (sizeLength < longestSizeLabelLength) {
			const rightPadding = ' '.repeat(longestSizeLabelLength - sizeLength);
			sizeLabel += rightPadding;
		}
		console.log(` ${sizeLabel} ${chalk.dim(asset.folder + path.sep)}${chalk.cyan(asset.name)}`);
	});
}

// Create the production build and print the deployment instructions.
function build(previousSizeMap) {
	console.log('Creating an optimized production build...');
	webpack(config).run((err, stats) => {
		if (err) {
			console.error('Failed to create a production build. Reason:');
			console.error(err.message || err);
			process.exit(1);
		}

		console.log(chalk.green('Compiled successfully.'));
		console.log();

		console.log('File sizes after gzip:');
		console.log();
		printFileSizes(stats, previousSizeMap);
		console.log();
	});
}

// First, read the current file sizes in build directory.
// This lets us display how much they changed later.
recursive(buildPath, (err, fileNames) => {
	const previousSizeMap = (fileNames || [])
		.filter(fileName => /\.(js|css)$/.test(fileName))
		.reduce((memo, fileName) => {
			const contents = fs.readFileSync(fileName);
			const key = removeFileNameHash(fileName);
			memo[key] = gzipSize(contents);
			return memo;
		}, {});

	// Remove all content but keep the directory so that
	// if you're in it, you don't end up in Trash
	rimrafSync(`${buildPath}/*`);

	// Start the webpack build
	build(previousSizeMap);
});