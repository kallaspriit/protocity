import { createStore, combineReducers, applyMiddleware, compose } from 'redux';
import thunkMiddleware from 'redux-thunk';
// import promiseMiddleware from 'redux-loading-promise-middleware';
import promiseMiddleware from './services/promiseMiddleware';

// reducers
import gatewayReducer from './common/gateway/gatewayReducer';

export default function configureStore(initialState) {
	return createStore(
		combineReducers({
			gatewayReducer,
		}),
		initialState,
		compose(
			applyMiddleware(thunkMiddleware),
			applyMiddleware(promiseMiddleware),
			process.env.NODE_ENV === 'development' && window.devToolsExtension ? window.devToolsExtension() : f => f,
		),
	);
}
