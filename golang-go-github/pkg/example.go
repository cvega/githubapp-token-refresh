package main

import (
	"context"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"strings"
	"time"

	"github.com/bradleyfalzon/ghinstallation"
	"github.com/google/go-github/v29/github"
	"golang.org/x/oauth2"

	"gopkg.in/yaml.v2"
)

func handle(err error) {
	if err != nil {
		log.Fatal(err)
	}
}

// Org represents each entry in .orgs of .config.yml
type Org struct {
	Name           string `yaml:"name"`
	Pem            string `yaml:"pem"`
	AppID          int64  `yaml:"app_id"`
	InstallationID int64  `yaml:"installation_id"`
}

// Config represents the root level of .config.yml
type Config struct {
	Orgs []Org `yaml:"orgs"`
}

// GitHubAPIAuth for API Authentication
type GitHubAPIAuth struct {
	meta       *Org
	privateKey string
	auth       *github.InstallationToken
	client     *github.Client
	ctx        context.Context
}

// Auth refreshes the client if:
//		- it hasn't been setup yet
//		- token is expired
func (g *GitHubAPIAuth) Auth() {
	if g.client == nil || time.Now().Unix()+60 > g.auth.ExpiresAt.Unix() {
		fmt.Println("- TOKEN STATUS: EXPIRED")
		g.GitHubClient()
		fmt.Println("- TOKEN STATUS: REFRESHED")
	}
	fmt.Println("- TOKEN STATUS: CURRENT")
	fmt.Printf("- TOKEN EXPIRY: %s\n", g.auth.ExpiresAt.Format(time.RFC3339))
	fmt.Printf("- TOKEN: %s%s\n", (*g.auth.Token)[:10], strings.Repeat("*", 30))
}

func (g *GitHubAPIAuth) newAppTransport() (*ghinstallation.AppsTransport, error) {
	return ghinstallation.NewAppsTransportKeyFromFile(http.DefaultTransport, g.meta.AppID, g.meta.Pem)
}

func (g *GitHubAPIAuth) newAppClient() (*github.Client, error) {
	at, err := g.newAppTransport()
	handle(err)
	return github.NewClient(&http.Client{Transport: at}), nil
}

func (g *GitHubAPIAuth) newInstallationToken() {
	appClient, err := g.newAppClient()
	handle(err)
	g.auth, _, err = appClient.Apps.CreateInstallationToken(g.ctx, g.meta.InstallationID, nil)
	handle(err)
}

func (g *GitHubAPIAuth) newTokenSource() oauth2.TokenSource {
	return oauth2.StaticTokenSource(
		&oauth2.Token{
			AccessToken: *g.auth.Token,
		},
	)
}

// Token returns an oauth2 client for creating a new github client
func (g *GitHubAPIAuth) Token() (*http.Client, error) {
	return oauth2.NewClient(g.ctx, g.newTokenSource()), nil
}

// GitHubClient creates a new github client
func (g *GitHubAPIAuth) GitHubClient() {
	g.newInstallationToken()
	httpClient, err := g.Token()
	handle(err)
	g.client = github.NewClient(httpClient)
}

// GitHubAPI for API requests
type GitHubAPI struct {
	*GitHubAPIAuth
}

// NewGitHubAPI returns a new instance
func NewGitHubAPI(meta *Org) *GitHubAPI {
	return &GitHubAPI{&GitHubAPIAuth{meta: meta, ctx: context.Background()}}
}

// OrgRepos returns a list of repositories with the app installed
func (g *GitHubAPI) OrgRepos() []*github.Repository {
	g.Auth()
	repos, _, err := g.client.Repositories.ListByOrg(g.ctx, g.meta.Name, &github.RepositoryListByOrgOptions{
		Type: "private",
	})
	handle(err)
	return repos
}

func main() {
	content, err := ioutil.ReadFile(".config.yml")
	handle(err)
	config := Config{}
	handle(yaml.Unmarshal(content, &config))

	for _, meta := range config.Orgs {
		g := NewGitHubAPI(&meta)
		fmt.Println("EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND GO-GITHUB")
		fmt.Printf("ctrl-c to exit; otherwise runs infinitely\n\n")
		for {
			fmt.Printf("- TIMESTAMP: %s\n", time.Now().UTC().Format(time.RFC3339))
			for _, repo := range g.OrgRepos() {
				fmt.Printf("- REPO OBJECT: %s\n\n", *repo.FullName)
				break
			}
			time.Sleep(15 * time.Minute)
		}
	}
}
