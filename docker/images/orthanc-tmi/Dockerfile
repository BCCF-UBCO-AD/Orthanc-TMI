FROM archlinux:latest

MAINTAINER Brian Zhou <brianzhouzc@gmail.com>
LABEL Description="Orthanc, free DICOM server, with plugins and other dependencies" Vendor="Orthanc-TMI"

WORKDIR /root/

ADD ./setup-environment.sh ./setup-environment.sh
RUN bash ./setup-environment.sh

RUN rm ./setup-environment.sh

VOLUME [ "/var/lib/orthanc/db" ]
EXPOSE 4242
EXPOSE 8042

ENTRYPOINT [ "Orthanc" ]
CMD [ "/etc/orthanc/" ]

# https://groups.google.com/d/msg/orthanc-users/qWqxpvCPv8g/Z8huoA5FDAAJ
ENV MALLOC_ARENA_MAX 5