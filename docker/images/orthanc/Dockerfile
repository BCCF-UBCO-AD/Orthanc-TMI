FROM archlinux:latest

MAINTAINER Brian Zhou <brianzhouzc@gmail.com>
LABEL Description="Orthanc, free DICOM server, with plugins and other dependencies" Vendor="Orthanc-TMI"

WORKDIR /root/
RUN pacman -Sy dos2unix --noconfirm

ADD ./setup-environment.sh ./setup-environment.sh
RUN dos2unix ./setup-environment.sh
RUN bash ./setup-environment.sh
RUN rm ./setup-environment.sh


ADD ./download-plugins.sh ./download-plugins.sh
RUN dos2unix ./download-plugins.sh
RUN bash ./download-plugins.sh
RUN rm ./download-plugins.sh

VOLUME [ "/var/lib/orthanc/db" ]
EXPOSE 4242
EXPOSE 8042

ENTRYPOINT [ "Orthanc" ]
CMD [ "/etc/orthanc/" ]

# https://groups.google.com/d/msg/orthanc-users/qWqxpvCPv8g/Z8huoA5FDAAJ
ENV MALLOC_ARENA_MAX 5