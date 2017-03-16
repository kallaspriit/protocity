// Grab NODE_ENV and APP_* environment variables and prepare them to be
// injected into the application via DefinePlugin in Webpack configuration.

const APP = /^APP_/i;

export default function getClientEnvironment(publicUrl) {
	const config = {
		NODE_ENV: JSON.stringify(process.env.NODE_ENV || 'development'),
		PUBLIC_URL: JSON.stringify(publicUrl),
		APP_API_URL: JSON.stringify(process.env.API_URL || 'https://jsonplaceholder.typicode.com/'),
	};

	const processEnv = Object
		.keys(process.env)
		.filter(key => APP.test(key))
		.reduce((env, key) => ({
			...env,
			[key]: JSON.stringify(process.env[key]),
		}), config);

	return { 'process.env': processEnv };
}
