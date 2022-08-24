mongo -- "$MONGO_INITDB_DATABASE" <<EOF
db.createUser({
  user: "$DB_USER_NAME",
  pwd: "$DB_USER_PASSWORD",
  roles: [
    {
      role: "readWrite",
      db: "$DB_NAME",
    },
  ],
});
EOF
