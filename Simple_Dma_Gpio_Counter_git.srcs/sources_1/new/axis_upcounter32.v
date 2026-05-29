`timescale 1ns / 1ps

module axis_upcounter32 (
    input  wire           aclk,
    input  wire           aresetn,

    // Enable (GPIO switch)
    input  wire           enable,

    // AXI Stream Master
    output reg  [31:0]    m_axis_tdata,
    output wire [3:0]     m_axis_tkeep,
    output reg            m_axis_tlast,
    output reg            m_axis_tvalid,
    input  wire           m_axis_tready,

    input  wire [31:0]    packet_size
);

    // All bytes valid
    assign m_axis_tkeep = 4'b1111;

    // AXI handshake
    wire axis_handshake = m_axis_tvalid && m_axis_tready;

    always @(posedge aclk or negedge aresetn) begin
        if (!aresetn) begin
            m_axis_tdata  <= 32'd0;
            m_axis_tvalid <= 1'b0;
            m_axis_tlast  <= 1'b0;
        end
        else begin
            // -------------------------------------------------
            // VALID CONTROL (AXI SAFE)
            // -------------------------------------------------
            if (enable) begin
                m_axis_tvalid <= 1'b1;
            end
            else if (axis_handshake) begin
                // Drop VALID only after a completed transfer
                m_axis_tvalid <= 1'b0;
            end

            // -------------------------------------------------
            // DATA + TLAST (ONLY ON HANDSHAKE)
            // -------------------------------------------------
            if (axis_handshake) begin

                // Packet size = 1 (special case)
                if (packet_size == 32'd1) begin
                    m_axis_tdata <= 32'd0;
                    m_axis_tlast <= 1'b1;
                end

                // Last beat of packet
                else if (m_axis_tdata == packet_size - 1) begin
                    m_axis_tdata <= 32'd0;
                    m_axis_tlast <= 1'b0;
                end

                // Pre-last beat ? assert TLAST
                else if (m_axis_tdata == packet_size - 2) begin
                    m_axis_tdata <= m_axis_tdata + 1'b1;
                    m_axis_tlast <= 1'b1;
                end

                // Normal count
                else begin
                    m_axis_tdata <= m_axis_tdata + 1'b1;
                    m_axis_tlast <= 1'b0;
                end
            end
        end
    end

endmodule