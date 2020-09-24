/*
 * SQL Schema for FIM registry database
 * Copyright (C) 2015-2020, Wazuh Inc.
 *
 * This program is a free software, you can redistribute it
 * and/or modify it under the terms of GPLv2.
 */

CREATE TABLE IF NOT EXISTS registry_key (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    path TEXT NOT NULL UNIQUE,
    perm TEXT,
    uid INTEGER,
    gid INTEGER,
    user_name TEXT,
    group_name TEXT,
    mtime INTEGER,
    arch TEXT CHECK (arch IN ('[x32]', '[x64]')),
    scanned INTEGER,
    checksum TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS key_path_index ON registry_key (path);

CREATE TABLE IF NOT EXISTS registry_data (
    key_id INTEGER,
    name TEXT,
    type INTEGER,
    size INTEGER,
    hash_md5 TEXT,
    hash_sha1 TEXT,
    hash_sha256 TEXT,
    scanned INTEGER,
    last_event INTEGER,
    checksum TEXT NOT NULL,

    PRIMARY KEY(key_id, name)
    FOREIGN KEY (key_id) REFERENCES registry_key(id)
);

CREATE INDEX IF NOT EXISTS key_name_index ON registry_data (key_id, name);

CREATE VIEW IF NOT EXISTS sync_view (path, checksum) AS
  SELECT arch || path || '\\' || name, registry_data.checksum FROM registry_key INNER JOIN registry_data ON registry_key.id=registry_data.key_id
  UNION ALL
  SELECT path, checksum FROM file_entry INNER JOIN file_data ON file_entry.inode_id=file_data.rowid;
