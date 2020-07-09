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
      let now = new Date();
      if (this.accessToken == undefined || now + 60 >= this.expiresAt) {
        console.log('- TOKEN STATUS: EXPIRED');
        return this.github_app.then(success);
      } else {
        return success();
      }
    })
    .then(() => {
      console.log('- TOKEN STATUS: CURRENT');
      console.log(`- TOKEN EXPIRY: ${this.expiresAt}`);
      console.log(`- TOKEN: ${this.accessToken.slice(0, 10) + '*'.repeat(30)}`);
      return this;
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
      console.log('- TOKEN STATUS: REFRESHED');
      this.accessToken = response.data.token;
      this.expiresAt = new Date(response.data.expires_at);
      return this;
    })
  }
}

class GitHubAPI extends GitHubAPIAuth {
  repos(callback) {
    return this.auth.then(client => {
      return request({
        method: 'GET',
        url: '/orgs/:org/repos',
        org: this.meta.name,
        headers: {
          authorization: `token ${client.accessToken}`,
          accept: "application/vnd.github.machine-man-preview+json",
        },
      }).then(callback);
    })
  }
}

yaml.safeLoad(fs.readFileSync('.config.yml', 'utf8')).orgs.forEach(meta => {
  gh = new GitHubAPI(meta);
  console.log('EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND @OCTOKIT/REST');
  console.log('ctrl-c to exit; otherwise runs infinitely\n');
  setInterval(() => {
    console.log(`- TIMESTAMP: ${new Date()}`);
    gh.repos(repos => {
      console.log(`- REPO OBJECT: ${repos.data[0].full_name}\n`);
    });
  }, 1000 * 5); // every 5 seconds
});
