# frozen_string_literal: true

require 'lib/octokit-autorefresh'

require 'yaml'

# run with: rspec -I . spec spec/octokit-autorefresh-spec.rb

describe GitHubAPI do
  before :all do
    @config = YAML.safe_load(File.read('.config.yml'))
  end

  before :each do
    @ghapi = GitHubAPI.new @config['orgs'][0]
  end

  context 'When calling GithubAPI.org_repos' do
    it 'should return one repo' do
      expect(@ghapi.org_repos.length).to eq(1)
    end
  end
end
