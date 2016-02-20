# stream-broadcast
Broadcast one stdin stream to many clients. It creates linux-socket file, listens clients to connect to this socket, transfers all data from it's stdin to any connected client.

Main purpose (but not only): to start loading video stream without playing it and then start and stop playing without interrupting loading.
You can start loading in one terminal and redirect output into `stream-broadcast` (specifying socket file). Then in another terminal you can start playing by `socat UNIX-CONNECT:linux_socket_filename` and you can interrupt it and start playing again as many times as you want.
Using `stream-broadcast` makes sense for self-synchronizing streams, for example for mpegts video container.

## Compilation
```
make
```
## Usage
```
some_stream | stream-broadcast linux_socket_filename
    CTRL+C for interrupt
```
## Example
```
(in one terminal)
ffmpeg -i 'http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8' -c copy -f mpegts - | stream-broadcast stream.socket
(in another terminal)
socat UNIX-CONNECT:stream.socket - | mpv -

```

