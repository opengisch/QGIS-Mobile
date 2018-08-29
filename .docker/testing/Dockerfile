FROM qgis/qgis:latest
MAINTAINER Matthias Kuhn <matthias@opengis.ch>

RUN apt-get update && \
    apt-get -y install \
      libqt5sensors5-dev \
      qtquickcontrols2-5-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /

