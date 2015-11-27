create table if not exists users(
id integer(32) primary key auto_increment,
login varchar(256) unique not null,
pwdhash varchar(256) not null,
email varchar(256) unique not null
);

create table if not exists msgs(
senderuid integer(32),
receiveruid integer(32),
msg text not null,
status bool not null default 0
);

create table if not exists tokens(
userid integer(32) unique not null,
token varchar(256) unique not null,
expiry datetime not null
);
