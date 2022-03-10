Please Note: 
  You need to copy this action to your project,
  as long as this action is not part of the ``actions`` GitHub organization.
  For example, copy it to ``.github/actions/repo-sync`` and use it with a
  ``use: ./.github/actions/repo-sync`` declaration
  
Repo Sync Action
================

This action allows the git branch synchronization from a GitHub for Enterprise
repo to a GitHub.com repo.
It uses a user-defined, private GitHub App for permission management and authentication.
See `Configuration` below to get started.

Usage
-----
Sample::

     on:
       push:
         branches:
           - 'my_branch'
     jobs:
        sync_my_branch:
          name: Sync `my_branch` with remote github.com repo
          runs-on: Linux
          # this environment must contain the action inputs as secrets
          # also ensure, that this environment is pinned to the local branch 'my_branch' 
          environment: repo-sync-environment
          steps:
            - uses: actions/checkout@v1
            - uses: actions/repo-sync@master
              with:
                app-key:         ${{ secrets.APP_KEY }}
                app-id:          ${{ secrets.APP_ID }}
                remote-repo-url: ${{ secrets.REMOTE_REPO_URL }}
                remote-branch:   ${{ secrets.REMOTE_BRANCH }}
                local-branch:    origin/my_branch


Inputs
------

``remote-repo-url``
^^^^^^^^^^^^^^^^^^
The ``HTTPS`` URL of the remote, GitHub.com repo.

``app-id``
^^^^^^^^^^
The GitHub App identifier.

``app-key``
^^^^^^^^^^^
A private PEM certificate associated with the GitHub App identified by ``app-id``.

``local-branch``
^^^^^^^^^^^^^^^^
The local git branch to push to the remote.

``remote-branch``
^^^^^^^^^^^^^^^^^
The remote target branch to push to.

Outputs
-------
None


Configuration
-------------

We use ``https`` for git transport and a GitHub App for creating authentication tokens.
For security reasons, the inputs to this action should be in environment secrets pinned to a specific branch.

Setup:

1. Create a GitHub App on your **https://github.com** account.
   Go to ``Settings > Developer Settings > GitHub Apps`` and click ``New GitHub App``.
   Keep this Application private and note down the App ID (required as input for this action).
   Assign the ``contents`` permission to this App.
   
   Note:
       If your git branch contains a ``.github`` folder, you will also need the ``workflow`` permission.
   Note:
       Syncing to repos by a different owner require a public GitHub App.
       Otherwise, you can trnasfer ownership of your private GitHub App to the repo's organization.
   
   This App will be used for authenticating all git operations between the local and remote repos.
   
2. In the GitHub App settings, create a private key.
   A private certificate (acting as a key) will be generated and offered for download.

   Note:
       Never commit this file into git history, we create a GitHub repo secret to contain this key safely.

3. Install your newly created GitHub App into the target remote repo (the one denoted with the input ``remote-repo-url``).
   Go to the App's Settings page via your account's ``Settings > Developer Settings > Github Apps > $your-app > Install App``.
   Note that if your repo's owner is an organization, you'll have to create the App as 'public' and install it into your organization,
   limiting the installation to the target repo in the organization.
   Otherwise your orga's repo won't show up in the installation list.

4. In your local GitHub for Enterprise repo, create a new **environment**, e.g. ``repo-sync-environment``
   Then create secrets for all the required repo-sync inputs.
   For the ``app-key`` input, create a secret with the contents of the private key file, e.g. *APP_KEY*.
   
   For the remainder of this guide, we assume you created secrets for all inputs.
   
   
4. After creating all required secrets in the ``repo-sync-environment`` we can use it in the workflow
   which uses this action, see _Usage for reference.
   
