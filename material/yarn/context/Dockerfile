FROM node:16-alpine
ENV NODE_ENV development
WORKDIR /opt/sayok/expo

ARG uid=1000
ARG gid=1000

RUN chown $uid:$gid /opt/sayok/expo
COPY --chown=$uid:$gid . .

USER $uid:$gid

RUN yarn set version berry ; sed -i '/^nodeLinker:/d' .yarnrc.yml ; echo "nodeLinker: node-modules" >> .yarnrc.yml ; yarn

