import React from 'react';
import { render } from 'react-dom';
import { BrowserRouter as Router } from 'react-router-dom';
import { Provider } from 'react-redux';
import { AppContainer } from 'react-hot-loader';
import configureStore from './store';
import './gfx/main.scss';
import App from './App';

// https://github.com/gaearon/react-hot-loader/issues/462
delete AppContainer.prototype.unstable_handleError;

const rootElement = document.getElementById('root');
const store = configureStore();

function renderApp(content) {
	return (
		<AppContainer>
			<Provider store={store}>
				<Router>
					{content}
				</Router>
			</Provider>
		</AppContainer>
	);
}

render(
	renderApp(<App />),
	rootElement,
);

if (module.hot) {
	module.hot.accept('./App', () => {
		const NextApp = require('./App').default; // eslint-disable-line global-require

		render(
			renderApp(<NextApp />),
			rootElement,
		);
	});
}