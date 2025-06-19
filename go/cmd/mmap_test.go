package main

// func Test_Mmap(t *testing.T) {
// 	fileName := "testdata/mmap_testfile.txt"
// 	fstat, err := os.Stat(fileName)
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	mmapR, err := mmapOpen(fileName)
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	defer mmapR.Close()

// 	f, err := os.Open(fileName)
// 	if err != nil {
// 		t.Fatal(err)
// 	}

// 	fbufFull := make([]byte, fstat.Size())
// 	fbuf := make([]byte, fstat.Size()/3)
// 	var n int
// 	for {
// 		var err error
// 		n, err = f.Read(fbuf[n:])
// 		if err != nil {
// 			if err == io.EOF {
// 				break
// 			}
// 			t.Fatal(err)
// 		}
// 		copy(fbufFull[n:], fbuf)
// 	}
// 	n = 0

// 	rbufFull := make([]byte, fstat.Size())
// 	buf := make([]byte, fstat.Size()/3)
// 	for {
// 		var err error
// 		n, err = mmapR.Read(buf[n:])
// 		if err != nil {
// 			if err == io.EOF {
// 				break
// 			}
// 			t.Fatal(err)
// 		}
// 		copy(rbufFull[n:], buf)
// 	}

// 	assert.Equal(t, fbufFull, rbufFull, "mmap and file contents are not equal")
// 	assert.Equal(t, mmapR.readOff, fstat.Size(), "mmap offset is not 0")

// 	mmapR.readOff = fstat.Size()
// 	_, err = mmapR.Read(buf)
// 	assert.Equal(t, err, io.EOF, "read after offset at the end should return EOF")
// }
