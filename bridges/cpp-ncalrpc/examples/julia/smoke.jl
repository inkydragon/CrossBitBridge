# No package dependencies. Run with x64 Julia on Windows.
using InteractiveUtils
using Libdl
using Test

load_library(path::AbstractString) = Libdl.dlopen(abspath(path))
unload_library(library) = Libdl.dlclose(library)

# Keep the UTF-16 buffer alive by passing it directly to ccall.
function ncal_ping(library, endpoint::Vector{UInt16})
    bits = Ref{UInt32}(0)
    status = ccall(Libdl.dlsym(library, :ncal_ping), UInt32,
                   (Ptr{UInt16}, Ref{UInt32}), endpoint, bits)
    return (; status, bits = bits[])
end

function ncal_add(library, endpoint::Vector{UInt16}, a::Int32, b::Int32)
    result = Ref{Int32}(0)
    app = Ref{Int32}(-1)
    status = ccall(Libdl.dlsym(library, :ncal_add), UInt32,
                   (Ptr{UInt16}, Int32, Int32, Ref{Int32}, Ref{Int32}),
                   endpoint, a, b, result, app)
    return (; status, app = app[], result = result[])
end

function ncal_scale(library, endpoint::Vector{UInt16}, input::Vector{Float32}, factor::Float32)
    output = zeros(Float32, length(input))
    app = Ref{Int32}(-1)
    status = ccall(Libdl.dlsym(library, :ncal_scale), UInt32,
                   (Ptr{UInt16}, UInt32, Ptr{Float32}, Float32, Ptr{Float32}, Ref{Int32}),
                   endpoint, length(input), input, factor, output, app)
    return (; status, app = app[], output)
end

function test_library(library, endpoint::AbstractString)
    versioninfo()
    endpoint_buffer = transcode(UInt16, endpoint * "\0")
    @testset "Julia x64 -> client DLL -> ncalrpc -> x86 C++ DLL" begin
        ping = ncal_ping(library, endpoint_buffer)
        @test ping.status == 0 && ping.bits == 32

        add = ncal_add(library, endpoint_buffer, Int32(19), Int32(23))
        @test add.status == 0 && add.app == 0 && add.result == 42

        input = Float32[1, -2, 0, 3.5]
        scale = ncal_scale(library, endpoint_buffer, input, 2.0f0)
        @test scale.status == 0 && scale.app == 0
        @test scale.output == Float32[2, -4, 0, 7]
    end
    println("PASS: Julia x64 -> client DLL -> ncalrpc -> x86 C++ DLL")
    return nothing
end

function main(args = ARGS)
    Sys.iswindows() && Sys.WORD_SIZE == 64 || error("Requires x64 Windows Julia")
    length(args) == 2 || error("Usage: smoke.jl <absolute-client-dll> <endpoint>")
    library = load_library(args[1])
    try
        test_library(library, args[2])
    finally
        unload_library(library)
    end
    return nothing
end

if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
