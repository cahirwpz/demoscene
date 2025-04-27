#define {{ .Name }}_colors_count {{ .Count }}

{{if not .Shared }}static {{ end }}__data {{if .Aga}}u_int{{else}}u_short{{end}} {{ .Name }}_colors[{{ .Count }}] = {
  {{ range .ColorsData }}
  {{- . -}},
  {{ end -}}
};

