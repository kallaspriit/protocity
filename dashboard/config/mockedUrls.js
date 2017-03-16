import { required, minLength } from '../src/services/validatorService';

const getAddCommentErrors = ({ name, body }) => {
	const validationErrors = {
		name: required(name) || minLength(5)(name),
		body: required(body),
	};

	return Object.values(validationErrors).filter(value => Boolean(value)).length > 0
		? { validationErrors }
		: false;
};

export default [
	{
		regex: /comments\?postId=\d/,
		response: '/mockData/comments.json',
	},
	{
		regex: /comments\/add\?postId=\d/,
		response: body => ({
			...body,
			id: Math.round(Math.random() * 1000000),
		}),
		error: getAddCommentErrors,
		timeout: 1000,
	},
];