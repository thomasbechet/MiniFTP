# FTP implementation written in C

## Start server
```console
sftp_srv <service>
sftp_srv 55555
```
## Start client
```console
sftp_clt <address> <service>
sftp_clt 0.0.0.0 55555
```

## Commands (client)
```console
help            : display all commands.
quit            : close the connection and leave.
exit            : close the connection and leave.
get <src> <dst> : Retrieve a file from server.
put <src> <dst> : Transfer a file to the server.
ls  <dir>       : Display a distant directory.
```
