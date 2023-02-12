ARG OXVIID=v0
FROM node:14.18-alpine3.12 as build
WORKDIR /ox_perf
COPY . /ox_perf
RUN apk add build-base linux-headers
RUN make

FROM node:14.18-alpine3.12 as production
COPY --from=build /ox_perf/ox_perf /usr/local/bin