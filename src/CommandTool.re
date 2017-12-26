open Node;

let hasOption = (name: string) => Process.argv |> Js.Array.includes(name);