---
name: github-uploader
description: >-
  Manage GitHub repositories, programmatic repository creation, Git authentication,
  and project deployment using GitHub Personal Access Tokens (Classic PAT) and Git CLI.
---

# GitHub Project Management & Uploader Skill

This skill documents and automates managing GitHub repositories, authenticating Git operations, and pushing codebases to GitHub using GitHub Personal Access Tokens (PAT).

## Stored Credentials
The GitHub credentials are configured in `.env` (project root and `~/.env`):
- `GITHUB_USERNAME`: `Sibi9`
- `GITHUB_TOKEN`: Classic Personal Access Token with `repo`, `workflow`, and administrative scopes.

## How to Use the Token

### 1. In Git CLI (Push / Clone / Remote)
When interacting with remote repositories over HTTPS, use the token directly in the remote URL or authorization header:

```bash
# Setting remote with embedded token
git remote set-url origin https://Sibi9:<GITHUB_TOKEN>@github.com/Sibi9/<REPO_NAME>.git

# Pushing branches with live progress
git push -u origin main --progress
```

### 2. In GitHub REST API (Creating Repositories)
To create a new public or private repository programmatically without opening a browser:

```python
import urllib.request
import json
import os

token = os.environ.get("GITHUB_TOKEN", "<YOUR_TOKEN>")
repo_name = "FartTogether-CustomEngine"

payload = json.dumps({
    "name": repo_name,
    "description": "1:1 Authentic C++17 Custom Native Engine and Assets for Fart Together",
    "private": False, # Public repository
    "has_issues": True,
    "has_projects": True,
    "has_wiki": True
}).encode("utf-8")

req = urllib.request.Request("https://api.github.com/user/repos", data=payload, method="POST")
req.add_header("Authorization", f"token {token}")
req.add_header("Accept", "application/vnd.github+json")
req.add_header("Content-Type", "application/json")
req.add_header("User-Agent", "GitHubUploader/1.0")

with urllib.request.urlopen(req) as resp:
    data = json.loads(resp.read().decode())
    print("Repository URL:", data["html_url"])
```

### 3. In cURL
```bash
curl -X POST \
  -H "Authorization: token <GITHUB_TOKEN>" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/user/repos \
  -d '{"name":"RepoName","private":false}'
```

### 4. Interactive Progress Monitor
To track upload speed, transferred bytes, and estimated remaining time, run:
```bash
py upload_to_github.py
```
or launch:
```bash
launch_uploader.bat
```

