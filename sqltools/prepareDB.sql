create table if not exists users(
id integer(64) primary key auto_increment,
login varchar(256) unique not null,
pwdhash varchar(256) not null,
email varchar(256) unique not null,
dateregister datetime not null
);

create table if not exists msgs(
id integer(64) primary key auto_increment,
senderuid integer(64),
receiveruid integer(64),
msg text not null,
datesent datetime not null,
status bool not null default 0
);

create table if not exists tokens(
userid integer(32) unique not null,
token varchar(256) unique not null,
expiry datetime not null
);
