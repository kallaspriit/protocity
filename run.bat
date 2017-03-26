git pull

pushd dashboard
call npm i
popd

pushd scripts
call npm i
call npm start
popd