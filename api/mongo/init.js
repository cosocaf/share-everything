db.createUser({
  user: "local_user",
  pwd: "local_password",
  roles: [
    {
      role: "readWrite",
      db: "share-everything",
    },
  ],
});
