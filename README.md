AcoustID utility functions for PostgreSQL
=========================================

You need the PostgresSQL development page to build this:

```sh
sudo apt-get install postgresql-server-dev-12
```

If you have multiple PostgreSQL versions you need to specify which version to
build against by setting the `PG_CONFIG` variable:

```sh
export PG_CONFIG=/usr/lib/postgresql/12/bin/pg_config
```

Build and install the module:

```sh
make
sudo make install
```

Run tests:

```
sudo make installcheck PGUSER=postgres
```

Using in PostgreSQL:

```sql
CREATE EXTENSION acoustid;
```

If you have a version from before PostgreSQL 9.1 installed, you can upgrade it
to a properly package one with:

```sql
CREATE EXTENSION acoustid FROM unpackaged;
```
