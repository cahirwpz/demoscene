all:
	go build

windows:
	GOOS=windows GOARCH=amd64 go build

clean:
	go clean
	rm -f *~
