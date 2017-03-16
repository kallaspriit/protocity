import { handleActions } from 'redux-actions';
import * as homeViewConstants from './homeViewConstants';

const initialState = {
	posts: [],
	isPostsLoading: false,
	error: null,
};

export default handleActions({
	[homeViewConstants.GET_POSTS_BY_USER_ID]: (state, action) => {
		const {
			error,
			isLoading: isPostsLoading,
			payload,
		} = action;

		return {
			...state,
			isPostsLoading,
			error,
			posts: Array.isArray(payload) ? payload : [],
		};
	},
}, initialState);