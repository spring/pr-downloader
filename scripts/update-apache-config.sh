#!/bin/bash


GITROOT=/home/packages/git
WWWROOT=/home/packages/www/repos.springrts.com
STREAMER_BIN=/home/packages/bin/Streamer
DOMAINS=$(find $GITROOT -maxdepth 1 -mindepth 1 -type d)
# 001- is prefixed for defined order of files
CONFIGFILE=/etc/apache2/sites-enabled/001-all.repo.springrts.com.conf

echo "" > $CONFIGFILE

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



echo "Appending $CONFIGFILE"
cat >>$CONFIGFILE << EOF

# don't edit! automaticly created with $(realpath $0)
<VirtualHost *:80>
    AssignUserID packages packages
    ServerName $TAG.repo.springrts.com
    ErrorLog /var/log/apache2/$TAG.repo.springrts.com-error.log
    CustomLog /var/log/apache2/$TAG.repo.springrts.com-access.log combined
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

done
