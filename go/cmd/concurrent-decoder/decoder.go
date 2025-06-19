package main

type seekTableFooter struct {
	numOfFrames         uint32
	seekTableDescriptor uint8
	magicNumber         uint32
}

type wenv struct {
	seekTableFooter *seekTableFooter
}

func main() {}
