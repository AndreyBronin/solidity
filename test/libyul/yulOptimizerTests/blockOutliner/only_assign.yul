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
//     { a := _1(c, b) }
//     { b := _1(a, c) }
//     { c := _1(b, a) }
//     function _1(c, b) -> a_2
//     {
//         a := mul(b, c)
//         a_2 := a
//     }
// }
