package main

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
)

type PaletteParams struct {
	Name        string
	Colors      int
	StoreUnused bool
}

func (pp *PaletteParams) String() string {
	return fmt.Sprintf("%s,%d,%t", pp.Name, pp.Colors, pp.StoreUnused)
}

func (pp *PaletteParams) Set(value string) error {
	params := strings.Split(value, ",")
	if len(params) < 2 {
		return errors.New("not enough comma separated parameters")
	}
	colors, err := strconv.Atoi(params[1])
	if err != nil {
		return errors.New("could not parse colors parameter to integer")
	}
	storeUnused := false
	if len(params) == 3 {
		storeUnused, err = strconv.ParseBool(params[2])
		if err != nil {
			return errors.New("could not parse store-unused param to boolean")
		}
	}
	*pp = PaletteParams{
		Name:        params[0],
		Colors:      colors,
		StoreUnused: storeUnused,
	}
	return nil
}
