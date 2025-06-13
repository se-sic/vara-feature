use std::ffi::CStr;
use std::fs::File;
use std::io::BufWriter;
use std::os::raw::c_char;

use oxidd::bdd::{new_manager, BDDManagerRef, BDDFunction};
use oxidd::ManagerRef;
use oxidd_dump::dot::dump_all;
use oxidd::BooleanFunction;

pub extern "C" fn dump_bdd(path: *const c_char) {
    // Convert C string to Rust string
    let c_str = unsafe {
        assert!(!path.is_null());
        CStr::from_ptr(path)
    };
    let path_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return,
    };

    // Create BDD manager and some BDD variables/functions
    let mgr_ref: BDDManagerRef = new_manager(32, 100, 1);

    // Create BDD variables/functions within the exclusive manager access
    let (f1, f2, f3) = mgr_ref.with_manager_exclusive(|m| {
        let f1 = BDDFunction::new_var(m).unwrap();
        let f2 = BDDFunction::new_var(m).unwrap();
        let f3 = f1.and(&f2).unwrap();
        (f1, f2, f3)
    });

    // Define variable names
    let vars = vec![
        (&f1, "x0".to_string()),
        (&f2, "x1".to_string())
    ];

    // Define function names
    let functions = vec![
        (&f3, "f3".to_string())
    ];

    // Write DOT output to file
    let file = match File::create(path_str) {
        Ok(f) => f,
        Err(_) => return,
    };
    let writer = BufWriter::new(file);

    mgr_ref.with_manager_shared(|mgr| {
        if let Err(e) = dump_all(writer, mgr, vars, functions) {
            eprintln!("Dump failed: {}", e);
        }
    });
}
