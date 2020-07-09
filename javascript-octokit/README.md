# :factory: :shower: Automatically refreshing GitHub App tokens with Javascript

#### **Required class initialization parameters**
```javascript
meta.pem, meta.app_id, meta.installation_id
```

### Example Javascript Auth Class
```javascript
const { App } = require("@octokit/app");
const { request } = require("@octokit/request");
const fs = require('fs');
const yaml = require('js-yaml');


class GitHubAPIAuth {
  constructor(meta) {
    this.meta = meta;
    this.private_key = fs.readFileSync('private_key.pem', 'utf8');
    this.app = new App({ id: this.meta.app_id, privateKey: this.private_key })
  }

  get auth() {
    return new Promise(success => {
      this.accessToken == undefined || new Date() + 60 >= this.expiresAt ?
        this.github_app.then(success) :
        success()
    });
  }

  get token() {
    return request("POST /app/installations/:installation_id/access_tokens", {
      installation_id: this.meta.installation_id,
      headers: {
        accept: "application/vnd.github.machine-man-preview+json",
        authorization: `bearer ${this.app.getSignedJsonWebToken()}`,
      },
    })
  }

  get github_app() {
    return this.token
    .then(response => {
      this.accessToken = response.data.token;
      this.expiresAt = new Date(response.data.expires_at);
      return;
    })
  }
}

class GitHubAPI extends GitHubAPIAuth {
  repos(callback) {
    return this.auth
    .then(_ => {
      return request({
        method: 'GET',
        url: '/orgs/:org/repos',
        org: this.meta.name,
        headers: {
          authorization: `token ${this.accessToken}`,
          accept: "application/vnd.github.machine-man-preview+json",
        },
      })
      .then(callback);
    })
  }
}

yaml.safeLoad(fs.readFileSync('.config.yml', 'utf8')).orgs.forEach(meta => {
  new GitHubAPI(meta).repos(repos => {
    console.log(`- REPO OBJECT: ${repos.data[0].full_name}\n`);
  });
});
```

### Octokit.rb NOTES
The `bearer` token is only valid for a single second in the example class. We assume when using a chained method that the subsequent requests will happen in the one second time frame, however that may not be a safe assumption and doesn’t account for things like latency. While I didn't experience any issues, you may want to bump this value. Last, if the token is within 60 seconds of expiring we can renew it, this mitigates edge cases where the token may be valid at the time of the check but not at the time of the API call. This would be rare, also but included for demonstration.
