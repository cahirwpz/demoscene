(*
 * Copyright 2021-2023 Piotr Polesiuk & Ghostown
 * SPDX-License-Identifier: MIT
 *)

type var = int

let var_num = ref 0

let fresh_var () =
  var_num := !var_num + 1;
  !var_num

type lit =
| Pos of var
| Neg of var

let pos x = Pos x
let neg x = Neg x

let clauses = ref []

let add_clause c =
  clauses := c :: !clauses

let dump_dimacs () =
  let dump_lit l =
    match l with
    | Pos n -> Printf.printf "%d " n
    | Neg n -> Printf.printf "-%d " n
  in
  Printf.printf "p cnf %d %d\n" !var_num (List.length !clauses);
  !clauses |> List.iter (fun cl ->
    List.iter dump_lit cl;
    Printf.printf "0\n")

let values = Hashtbl.create 32
let rec read_dimacs_loop () =
  let n = Scanf.scanf " %d" (fun n -> n) in
  if n = 0 then ()
  else begin
    if n > 0 then Hashtbl.add values n true
    else Hashtbl.add values (-n) false;
    read_dimacs_loop ()
  end

let read_dimacs () =
  match read_line () with
  | "SAT" ->
    read_dimacs_loop ()
  | l ->
    Printf.printf "%s\n" l;
    exit 1

type one_hot_sel =
| OHSel of var list

let one_hot_sel n =
  let xs = List.init n (fun _ -> fresh_var ()) in
  add_clause (List.map pos xs);
  List.iteri (fun i xi ->
    List.iteri (fun j xj ->
      if i > j then add_clause [ neg xi; neg xj ]
    ) xs
  ) xs;
  OHSel xs

let one_hot_mux (OHSel xs) ys =
  let z = fresh_var () in
  List.iter2 (fun x y ->
    add_clause [ neg x; neg y; pos z ];
    add_clause [ neg x; pos y; neg z ]
  ) xs ys;
  z

type lut =
| Lut of int * var list

let lut_create n =
  Lut(n, List.init (1 lsl n) (fun _ -> fresh_var ()))

let lut_lookup (Lut(n, xs)) ys =
  assert (List.length ys = n);
  let z = fresh_var () in
  List.iteri (fun i x ->
    let cnd = ys |> List.mapi (fun j y ->
      if ((i lsr j) land 1) = 0 then pos y else neg y) in
    add_clause ( neg x :: pos z :: cnd );
    add_clause ( pos x :: neg z :: cnd )
  ) xs;
  z

let input n =
  let tt = fresh_var () in
  let ff = fresh_var () in
  add_clause [ pos tt ];
  add_clause [ neg ff ];
  List.init (1 lsl n) (fun i ->
    let xs = List.init n (fun j ->
      if ((i lsr j) land 1) = 0 then ff else tt)
    in
    (xs, i))

let set_true x  = add_clause [ pos x ]
let set_false x = add_clause [ neg x ]

let minterms = ["NANBNC"; "NANBC"; "NABNC"; "NABC"; "ANBNC"; "ANBC"; "ABNC"; "ABC"]
let string_of_lut4 (Lut(_, xs)) =
  minterms |> List.mapi (fun i minterm ->
    let n_bits = (i land 0x1) + ((i lsr 1) land 0x1) + ((i lsr 2) land 0x1) in
    let x = List.nth xs n_bits in
    if Hashtbl.find values x then
      minterm
    else
      ""
  ) |> List.filter (fun s -> s <> "") |> String.concat " | "

let string_of_lut8 (Lut(_, xs)) =
  List.map2 (fun x minterm ->
    if Hashtbl.find values x then
      minterm
    else
      ""
  ) xs minterms |> List.filter (fun s -> s <> "") |> String.concat " | "

let one_hot_sel_n (OHSel xs) =
  xs
  |> List.mapi (fun i x -> if Hashtbl.find values x then i else 0)
  |> List.fold_left (+) 0

let lut_lookup_n (Lut(l, xs)) n =
  let x = List.nth xs n in
  if Hashtbl.find values x then 1 else 0
