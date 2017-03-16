import { get, post } from '../services/fetchService';

export const getPostsByUserId = id => get(`posts?userId=${id}`);

export const getPostById = id => get(`posts/${id}`);

export const getCommentsyPostId = postId => get(`comments?postId=${postId}`);

export const saveComment = (postId, data) =>	post(`comments/add?postId=${postId}`, data, true);