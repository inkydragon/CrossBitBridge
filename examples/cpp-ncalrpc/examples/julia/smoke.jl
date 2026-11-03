# No package dependencies. Run with x64 Julia on Windows.
using Libdl
Sys.iswindows() && Sys.WORD_SIZE == 64 || error("Requires x64 Windows Julia")
length(ARGS) == 2 || error("Usage: smoke.jl <absolute-client-dll> <endpoint>")
library = Libdl.dlopen(abspath(ARGS[1]))
try
    endpoint = transcode(UInt16, ARGS[2] * "\0")
    bits = Ref{UInt32}(0)
    status = ccall(Libdl.dlsym(library, :ncal_ping), UInt32,
                   (Ptr{UInt16}, Ref{UInt32}), endpoint, bits)
    @assert status == 0 && bits[] == 32
    result, app = Ref{Int32}(0), Ref{Int32}(-1)
    status = ccall(Libdl.dlsym(library, :ncal_add), UInt32,
                   (Ptr{UInt16}, Int32, Int32, Ref{Int32}, Ref{Int32}),
                   endpoint, 19, 23, result, app)
    @assert status == 0 && app[] == 0 && result[] == 42
    input = Float32[1, -2, 0, 3.5]
    output = zeros(Float32, length(input))
    status = ccall(Libdl.dlsym(library, :ncal_scale), UInt32,
                   (Ptr{UInt16}, UInt32, Ptr{Float32}, Float32, Ptr{Float32}, Ref{Int32}),
                   endpoint, length(input), input, 2, output, app)
    @assert status == 0 && app[] == 0 && output == Float32[2, -4, 0, 7]
    println("PASS: Julia x64 -> client DLL -> ncalrpc -> x86 C++ DLL")
    println("Julia ", VERSION, " / ", Sys.ARCH)
finally
    Libdl.dlclose(library)
end
