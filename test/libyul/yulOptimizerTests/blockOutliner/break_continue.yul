{
    let a := 1
    let b := 2
    let c := 3
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { break }
		}
	}
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { break }
		}
	}
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { break }
		}
	}
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { continue }
		}
	}
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { continue }
		}
	}
	{
		for {} 1 {} {
			{ a := mul(b,c) }
			if gt(a,b) { continue }
		}
	}
}
// ====
// step: blockOutliner
// ----
// {
//     let a := 1
//     let b := 2
//     let c := 3
//     { a := outlined$48$(a, b, c) }
//     { a := outlined$48$(a, b, c) }
//     { a := outlined$48$(a, b, c) }
//     { a := outlined$261$(a, b, c) }
//     { a := outlined$261$(a, b, c) }
//     { a := outlined$261$(a, b, c) }
//     function outlined$69$(b, c) -> a_1
//     {
//         a := mul(b, c)
//         a_1 := a
//     }
//     function outlined$48$(a, b, c) -> a_2
//     {
//         for { } 1 { }
//         {
//             { a := outlined$69$(b, c) }
//             if gt(a, b) { break }
//         }
//         a_2 := a
//     }
//     function outlined$261$(a, b, c) -> a_3
//     {
//         for { } 1 { }
//         {
//             { a := outlined$69$(b, c) }
//             if gt(a, b) { continue }
//         }
//         a_3 := a
//     }
// }
