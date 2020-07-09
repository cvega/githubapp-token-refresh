
let a = new Promise((success, error) => {
    success('test')
}).then(token => {
    console.log(`token: ${token}`)
    return Promise.resolve({newToken: 'newToken', test: 'test123'});
}).then((newToken, test) => {
    console.log(`new token: ${newToken}`)
    console.log(test);
})

