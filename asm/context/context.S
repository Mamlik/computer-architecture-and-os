        .intel_syntax noprefix
        .text
        
        .global savectx
savectx:
    mov rax, [rsp]
    mov [rdi], rax
    add rsp, 8
    mov [rdi+8], rsp
    sub rsp, 8
    mov [rdi+16], rbp
    mov rax, 0
    ret
       
       
        .global jumpctx
jumpctx:
    add rsp, 8
    mov rsp, [rdi+8]
    mov rbp, [rdi+16]
    mov rax, 1
    jmp [rdi]
    
         
          
          
          
               
               
               
                
        .att_syntax
