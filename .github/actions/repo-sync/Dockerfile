# Copyright (c) Vector Informatik GmbH. All rights reserved.
FROM ***VIB-820 Removed***/vector/ubuntu/full:20.04 as action-base

RUN apt-get update &&\
  DEBIAN_FRONTEND=noninteractive apt-get install -y \
  git \
  &&\
  rm -rf /var/lib/apt/lists/*

RUN pip3 install github3.py

COPY repo-sync.py /repo-sync.py
RUN chmod +rx /repo-sync.py

# workaround python 'requests' library not honoring our proxy/ssl settings:
ENV REQUESTS_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt 

ENTRYPOINT /repo-sync.py sync-branch ${REMOTE_REPO_URL} ${LOCAL_BRANCH} ${REMOTE_BRANCH}

