{
    let a
    let b
    let c
    {
        a := mul(b,c)
    }
    {
        b := mul(c,a)
    }
    {
        c := mul(a,b)
    }
}
// ====
// step: blockOutliner
// ----
// {
//     let a
//     let b
//     let c
//     { a := outlined$36$(c, b) }
//     { b := outlined$36$(a, c) }
//     { c := outlined$36$(b, a) }
//     function outlined$36$(c, b) -> a_1
//     {
//         a := mul(b, c)
//         a_1 := a
//     }
// }
