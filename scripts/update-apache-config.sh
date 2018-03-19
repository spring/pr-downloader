#!/bin/bash


GITROOT=/home/packages/git
WWWROOT=/home/packages/www
STREAMER_BIN=/home/packages/bin/Streamer
DOMAINS=$(find $GITROOT -maxdepth 1 -mindepth 1 -type d)

for DOMAIN in $DOMAINS; do

TAG=$(basename $DOMAIN)

cat > $WWWROOT/$TAG/index.html << EOF
<html>
<head>
	<title>spring rts repo</title>
</head>
<body>
For details see the <a href="https://springrts.com/wiki/Rapid">rapid wiki page</a>.
<br/>
See the <a href="log.txt">log file</a> for possible errors.
</body>
</html>
EOF

cat > $WWWROOT/$TAG/robots.txt << EOF
User-agent: *
Disallow: /
EOF

# 001- is prefixed for defined order of files
FILE=/etc/apache2/sites-enabled/001-$TAG.repo.springrts.com.conf

echo "Creating $FILE"
cat >$FILE << EOF

#Automaticly created with $(pwd)/$0, don't edit!
<VirtualHost *:80>
    AssignUserID packages packages
    ServerName $TAG.repo.springrts.com
    ErrorLog /var/log/apache2/$TAG.repo.springrts.com-error.log
    CustomLog /var/log/apache2/$TAG.repo.springrts.com-access.log combined
#    CustomLog /dev/null combined
    DocumentRoot $WWWROOT/$TAG
   <Directory $WWWROOT/$TAG>
        Require all granted
        AllowOverride all
    </Directory>
    <Location />
        RewriteEngine on
        AddHandler cgi-script .cgi
        Options +ExecCGI
    </Location>
    <Location /builds>
        Options +Indexes
    </Location>
</VirtualHost>
EOF

mkdir -p $WWWROOT/$TAG
chown packages:packages $WWWROOT/*
ln -svf $STREAMER_BIN $WWWROOT/$TAG/streamer.cgi
ln -svf $STREAMER_BIN $WWWROOT/$TAG/nph-streamer2.cgi

done
