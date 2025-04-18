(*
 * Copyright 2021-2023 Piotr Polesiuk & Ghostown
 * SPDX-License-Identifier: MIT
 *)

type var

val dump_dimacs : unit -> unit
val read_dimacs : unit -> unit

type one_hot_sel =
| OHSel of var list

val one_hot_sel : int -> one_hot_sel
val one_hot_mux : one_hot_sel -> var list -> var

type lut =
| Lut of int * var list

val lut_create : int -> lut
val lut_lookup : lut -> var list -> var

val input : int -> (var list * int) list

val set_true  : var -> unit
val set_false : var -> unit

val string_of_lut4 : lut -> string
val string_of_lut8 : lut -> string
val one_hot_sel_n : one_hot_sel -> int
val lut_lookup_n : lut -> int -> int
