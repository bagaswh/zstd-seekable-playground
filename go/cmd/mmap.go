package main

// func (r *mmapReader) Read(p []byte) (n int, err error) {
// 	n, err = r.ReadAt(p, r.readOff)
// 	r.readOff += int64(n)
// 	return
// }

// type mmapReader struct {
// 	data    []byte
// 	readOff int64
// }

// func (r *mmapReader) Close() error {
// 	if r.data == nil {
// 		return nil
// 	} else if len(r.data) == 0 {
// 		r.data = nil
// 		return nil
// 	}
// 	data := r.data
// 	r.data = nil
// 	runtime.SetFinalizer(r, nil)
// 	return syscall.Munmap(data)
// }

// func (r *mmapReader) Len() int {
// 	return len(r.data)
// }

// func (r *mmapReader) At(i int) byte {
// 	return r.data[i]
// }

// func (r *mmapReader) read(p *[]byte, off int64) (int, error) {
// 	if r.data == nil {
// 		return 0, errors.New("mmap: closed")
// 	}
// 	if off < 0 || int64(len(r.data)) < off {
// 		return 0, fmt.Errorf("mmap: invalid ReadAt offset %d", off)
// 	}
// 	// mmap read is just a normal memory read
// 	*p = r.data[off:]
// 	if n < len(p) {
// 		return n, io.EOF
// 	}
// 	return n, nil
// }

// var (
// 	errMmapClosed = errors.New("mmap: closed")
// )

// func (r *mmapReader) WriteTo(w io.Writer) (int64, error) {
// 	if r.data == nil {
// 		return 0, errMmapClosed
// 	}
// 	n, err := w.Write(r.data)
// 	return int64(n), err
// }

// func mmapOpen(filename string) (*mmapReader, error) {
// 	stat, err := os.Stat(filename)
// 	if err != nil {
// 		return nil, err
// 	}
// 	f, err := os.Open(filename)
// 	if err != nil {
// 		return nil, err
// 	}
// 	size := stat.Size()
// 	data, err := syscall.Mmap(int(f.Fd()), 0, int(size), syscall.PROT_READ, syscall.MAP_PRIVATE) // to avoid changing file contents, use MAP_PRIVATE
// 	if err != nil {
// 		return nil, err
// 	}
// 	r := &mmapReader{data, 0}
// 	runtime.SetFinalizer(r, (*mmapReader).Close)
// 	return r, nil
// }
