FROM node:current-buster-slim

# Setting working directory. All the path will be relative to WORKDIR
WORKDIR /usr/src/app
EXPOSE 3000

RUN mkdir logs

# Installing dependencies
ADD package.json ./

# ... this way we keep image really small (no intermediate containers)
RUN npm install

COPY . /usr/src/app
RUN echo "========= Prepared for build:"
RUN ls

RUN echo "========= Building"
RUN npm run build

RUN echo "========= Clearing up"
RUN rm -rf server tsconfig*

RUN echo "========= Cleaned up:"
RUN ls

# Is set during built
ARG AKUA_VERSION
ENV AKUA_VERSION=${AKUA_VERSION}

# It set at run
ENV AKUA_FORCE_DEV=""

# Running the app
CMD [ "./docker-utils/start" ]
