FROM brianzhouzc/orthanc-tmi:latest

MAINTAINER Brian Zhou <brianzhouzc@gmail.com>
LABEL Description="Orthanc, free DICOM server, with plugins and other dependencies" Vendor="Orthanc-TMI"

WORKDIR /root/

ADD ./download-plugins.sh ./download-plugins.sh
RUN bash ./download-plugins.sh

RUN rm ./download-plugins.sh

VOLUME [ "/var/lib/orthanc/db" ]
EXPOSE 4242
EXPOSE 8042

ENTRYPOINT [ "Orthanc" ]
CMD [ "/etc/orthanc/" ]

# https://groups.google.com/d/msg/orthanc-users/qWqxpvCPv8g/Z8huoA5FDAAJ
ENV MALLOC_ARENA_MAX 5