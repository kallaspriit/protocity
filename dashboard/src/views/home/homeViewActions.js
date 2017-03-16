import { createAction } from 'redux-actions';
import * as homeViewConstants from './homeViewConstants';
import * as placeholderApi from '../../api/placeholderApi';

export const getPostsByUserId = createAction(
	homeViewConstants.GET_POSTS_BY_USER_ID,
	placeholderApi.getPostsByUserId,
);