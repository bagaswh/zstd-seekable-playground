package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"os/signal"
	"syscall"

	seekable "github.com/SaveTheRbtz/zstd-seekable-format-go/pkg"
	"github.com/klauspost/compress/zstd"
)

func getCompressionLevel(level string) (zstd.EncoderLevel, error) {
	switch level {
	case "fastest":
		return zstd.SpeedFastest, nil
	case "fast":
		return zstd.SpeedBetterCompression, nil
	case "best":
		return zstd.SpeedBestCompression, nil
	case "default":
		return zstd.SpeedDefault, nil
	default:
		return 0, fmt.Errorf("unknown compression level")
	}
}

func compress(ctx context.Context, inputFile, outputFile, compressionLevel string, frameSize int) {
	var r io.Reader
	var f *os.File
	if inputFile == "" || inputFile == "-" {
		r = os.Stdin
	} else {
		var err error
		f, err = os.OpenFile(inputFile, os.O_RDONLY, 0644)
		if err != nil {
			log.Fatalf("failed to open input file: %s", err)
		}
		defer f.Close()
		r = f
	}
	_ = r

	var fout *os.File
	var _w io.Writer
	if outputFile == "" || outputFile == "-" {
		_w = os.Stdout
	} else {
		var err error
		fout, err = os.OpenFile(outputFile, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
		if err != nil {
			log.Fatalf("failed to open output file: %s", err)
		}
		defer fout.Close()
		_w = fout
	}

	zstdEncoderLevel, err := getCompressionLevel(compressionLevel)
	if err != nil {
		log.Fatalf("unknown compression level: %s", compressionLevel)
	}

	enc, err := zstd.NewWriter(nil,
		zstd.WithWindowSize(frameSize), zstd.WithEncoderLevel(zstdEncoderLevel))
	if err != nil {
		log.Fatalf("failed to create zstd encoder: %s", err)
	}
	defer enc.Close()

	seekableWriter, err := seekable.NewWriter(_w, enc)
	if err != nil {
		log.Fatalf("failed to create zstd encoder: %s", err)
	}
	defer seekableWriter.Close()

	readBufSize := max(frameSize, 10*1024*1024)
	buf := make([]byte, readBufSize)
	total := 0
	for {
		select {
		case <-ctx.Done():
			return
		default:
			n, err := r.Read(buf)
			start := 0
			for start < n {
				upto := min(frameSize, n-start)
				written, err := seekableWriter.Write(buf[start : start+upto])
				if err != nil {
					log.Fatal(err)
				}
				start += upto
				total += written
			}
			if err != nil {
				if err == io.EOF {
					return
				}
				log.Fatal(err)
			}
		}
	}

}

type seekTableFooter struct {
	numOfFrames         uint32
	seekTableDescriptor uint8
	magicNumber         uint32
}

type wenv struct {
	seekTableFooter *seekTableFooter
}

const (
	zstd_BlockSizeLog_Max uint64 = 17
	zstd_BlockSize_Max    uint64 = (1 << zstd_BlockSizeLog_Max)
)

// TODO: multithreaded decompression
func decompress(ctx context.Context, inputFile, outputFile string, threads int) {
	var _r io.ReadSeeker
	var f *os.File
	if inputFile == "" || inputFile == "-" {
		_r = os.Stdin
	} else {
		var err error
		f, err = os.OpenFile(inputFile, os.O_RDONLY, 0644)
		if err != nil {
			log.Fatalf("failed to open input file: %s", err)
		}
		defer f.Close()
		_r = f
	}

	var fout *os.File
	var w io.Writer
	if outputFile == "" || outputFile == "-" {
		w = os.Stdout
	} else {
		var err error
		fout, err = os.OpenFile(outputFile, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
		if err != nil {
			log.Fatalf("failed to open output file: %s", err)
		}
		defer fout.Close()
		w = fout
	}
	_ = w

	dec, err := zstd.NewReader(nil)
	if err != nil {
		log.Fatal(err)
	}
	defer dec.Close()

	r, err := seekable.NewReader(_r, dec)
	if err != nil {
		log.Fatal(err)
	}
	defer r.Close()

	// framesChan := make(chan int, 5)

	type decompressionResult struct {
		data           []byte
		err            error
		offset, length int
	}

	// frameSize := 0

	// g, ctx := errgroup.WithContext(ctx)
	// for i := 0; i < 1; i++ {
	// 	g.Go(func() error {
	// 		// total := 0
	buf := make([]byte, zstd_BlockSize_Max)
	off := int64(0)
	for {
		// fmt.Println("asdasd")
		n, readErr := r.ReadAt(buf, off)
		// start := 0
		// upto := min(frameSize, n-start)
		_, writeErr := w.Write(buf[:n])
		if writeErr != nil {
			log.Fatal(writeErr)
		}
		// start += n
		// total += written
		off += int64(n)

		if readErr != nil {
			if errors.Is(readErr, io.EOF) {
				break
			} else {
				log.Fatalf("error reading from offset %d: %v\n", off, readErr)
			}
			// return err
			// break
		}
	}
	// 	})
	// }

	// err = g.Wait()
	// if err != nil {
	// 	log.Fatal(err)
	// }
}

func main() {
	var inputFile string
	var outputFile string
	var compressionLevel string
	var frameSize int
	var doDecompress bool
	var decompressionThreads int

	flag.StringVar(&inputFile, "i", "", "Input file")
	flag.StringVar(&outputFile, "o", "", "Output file")
	flag.StringVar(&compressionLevel, "l", "fast", "Compression level")
	flag.IntVar(&frameSize, "s", 65536, "Frame size")
	flag.BoolVar(&doDecompress, "d", false, "Decompress")
	flag.IntVar(&decompressionThreads, "dt", 1, "Decompression threads")
	flag.Parse()

	ctx, cancel := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer cancel()
	go func() {
		<-ctx.Done()
		cancel()
		return
	}()

	exeName := os.Args[0]
	if exeName == "zstdseekcat" || doDecompress {
		decompress(ctx, inputFile, outputFile, decompressionThreads)
	} else {
		compress(ctx, inputFile, outputFile, compressionLevel, frameSize)
	}

	cancel()
}
