package misc

func UniqueUint32(ints []uint32) []uint32 {
	keys := make(map[uint32]bool)
	var list []uint32
	for _, entry := range ints {
		if _, value := keys[entry]; !value {
			keys[entry] = true
			list = append(list, entry)
		}
	}
	return list
}

func IndexOfUint32(element uint32, data []uint32) int {
	for k, v := range data {
		if element == v {
			return k
		}
	}
	return -1 //not found.
}
