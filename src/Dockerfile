FROM node:current-buster-slim

# Setting working directory. All the path will be relative to WORKDIR
WORKDIR /usr/src/app
EXPOSE 3000

RUN mkdir logs

# Installing dependencies
ADD package.json ./

ENV DEBIAN_FRONTEND=noninteractive

# ... this way we keep image really small (no intermediate containers)
RUN apt-get update \
    && apt-get install -y --no-install-recommends python autoconf automake make libtool nasm gcc "g++" udev \
    && npm install \
    && rm -rf /var/lib/apt/lists/* /etc/cron.d/* /etc/cron.daily/* /etc/cron.hourly/* /etc/cron.monthly/* /etc/cron.weekly/* \
    && echo "LANG=C.UTF-8" > /etc/default/locale \
    && dpkg --purge python autoconf automake libtool make nasm gcc "g++" udev \
    && apt-get clean \
    && apt-get -y autoremove \
    && apt-get purge -y --auto-remove -o APT::AutoRemove::RecommendsImportant=false

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
