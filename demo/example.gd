extends Node


func _ready() -> void:
    var example := ExampleClass.new()
    example.print_type(example)
    example.print_helloworld()
    var start = Time.get_ticks_usec()
    example.print_fibonacci(2**31-1)
    var end = Time.get_ticks_usec()
    print("Use Time（μs）:",end-start)
    
    var moonlignt = MoonlightStream.new()
    moonlignt.start_connection()
