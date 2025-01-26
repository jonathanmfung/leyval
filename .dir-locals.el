;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil .
      ((jf/project-compile-commands .
				 (("build-debug" . "cmake --preset debug && cmake --build --preset debug")
				  ("run-debug" . "cmake --preset debug && cmake --build --preset debug && ./build/debug/leyval")
				  ("test-debug" . "cmake --preset debug && cmake --build --preset debug && ctest --preset test-all")
				  ("build-release" . "cmake --preset release && cmake --build --preset release")
				  ("run-release" . "cmake --preset release && cmake --build --preset release && ./build/release/leyval")
				  )))))
