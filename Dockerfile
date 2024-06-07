FROM node:lts AS BUILD_IMAGE

WORKDIR /app

COPY . /app

RUN yarn install && \
    yarn run build

FROM node:lts-alpine

RUN rm -f /etc/apk/repositories && \
echo "https://mirror.reenigne.net/alpine/edge/community/" >> /etc/apk/repositories && \
echo "https://mirror.reenigne.net/alpine/edge/main/" >> /etc/apk/repositories && \
echo "https://mirror.reenigne.net/alpine/edge/testing/" >> /etc/apk/repositories && \
echo "https://mirror.reenigne.net/alpine/v3.19/main/" >> /etc/apk/repositories && \
echo "https://mirror.reenigne.net/alpine/v3.19/community/" >> /etc/apk/repositories && \
apk add --no-cache sox

COPY --from=BUILD_IMAGE /app/configs /app/configs
COPY --from=BUILD_IMAGE /app/package.json /app/package.json
COPY --from=BUILD_IMAGE /app/dist /app/dist
COPY --from=BUILD_IMAGE /app/public /app/public
COPY --from=BUILD_IMAGE /app/node_modules /app/node_modules

WORKDIR /app

EXPOSE 8000

CMD ["npm", "start"]