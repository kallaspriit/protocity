export const required = value => (value ? undefined : 'Field is required');

export const minLength = min => value =>
	(value && value.length < min ? `Must be at least ${min} characters` : undefined);
